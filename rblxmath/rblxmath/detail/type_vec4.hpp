/// @ref core
/// @file rblxmath/detail/type_vec4.hpp

#pragma once

#include "qualifier.hpp"
#if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
#	include "_swizzle.hpp"
#elif RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
#	include "_swizzle_func.hpp"
#endif
#include <cstddef>

namespace rblxmath
{
	template<typename T, qualifier Q>
	struct vec<4, T, Q>
	{
		// -- Implementation detail --

		typedef T value_type;
		typedef vec<4, T, Q> type;
		typedef vec<4, bool, Q> bool_type;

		// -- Data --

#		if RBLXMATH_SILENT_WARNINGS == RBLXMATH_ENABLE
#			if RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC
#				pragma GCC diagnostic push
#				pragma GCC diagnostic ignored "-Wpedantic"
#			elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#				pragma clang diagnostic push
#				pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#				pragma clang diagnostic ignored "-Wnested-anon-types"
#			elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#				pragma warning(push)
#				pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#			endif
#		endif

#		if RBLXMATH_CONFIG_XYZW_ONLY
			T x, y, z, w;
#			if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
			RBLXMATH_SWIZZLE_GEN_VEC_FROM_VEC4_COMP(T, Q, x, y, z, w)
#			endif//RBLXMATH_CONFIG_SWIZZLE
#		elif RBLXMATH_CONFIG_ANONYMOUS_STRUCT == RBLXMATH_ENABLE
			union
			{
				struct { T x, y, z, w; };
				struct { T r, g, b, a; };
				struct { T s, t, p, q; };

				typename detail::storage<4, T, detail::is_aligned<Q>::value>::type data;

#				if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
					RBLXMATH_SWIZZLE4_2_MEMBERS(T, Q, x, y, z, w)
					RBLXMATH_SWIZZLE4_2_MEMBERS(T, Q, r, g, b, a)
					RBLXMATH_SWIZZLE4_2_MEMBERS(T, Q, s, t, p, q)
					RBLXMATH_SWIZZLE4_3_MEMBERS(T, Q, x, y, z, w)
					RBLXMATH_SWIZZLE4_3_MEMBERS(T, Q, r, g, b, a)
					RBLXMATH_SWIZZLE4_3_MEMBERS(T, Q, s, t, p, q)
					RBLXMATH_SWIZZLE4_4_MEMBERS(T, Q, x, y, z, w)
					RBLXMATH_SWIZZLE4_4_MEMBERS(T, Q, r, g, b, a)
					RBLXMATH_SWIZZLE4_4_MEMBERS(T, Q, s, t, p, q)
#				endif
			};
#		else
			union { T x, r, s; };
			union { T y, g, t; };
			union { T z, b, p; };
			union { T w, a, q; };

#			if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
				RBLXMATH_SWIZZLE_GEN_VEC_FROM_VEC4(T, Q)
#			endif
#		endif

#		if RBLXMATH_SILENT_WARNINGS == RBLXMATH_ENABLE
#			if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#				pragma clang diagnostic pop
#			elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC
#				pragma GCC diagnostic pop
#			elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#				pragma warning(pop)
#			endif
#		endif

		// -- Component accesses --

		typedef length_t length_type;

		/// Return the count of components of the vector
		RBLXMATH_FUNC_DECL static RBLXMATH_CONSTEXPR length_type length(){return 4;}

		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T & operator[](length_type i);
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T const& operator[](length_type i) const;

		// -- Implicit basic constructors --

		RBLXMATH_DEFAULTED_DEFAULT_CTOR_DECL RBLXMATH_CONSTEXPR vec() RBLXMATH_DEFAULT_CTOR;
		RBLXMATH_DEFAULTED_FUNC_DECL RBLXMATH_CONSTEXPR vec(vec<4, T, Q> const& v) RBLXMATH_DEFAULT;
		template<qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<4, T, P> const& v);

		// -- Explicit basic constructors --

		RBLXMATH_CTOR_DECL explicit vec(T scalar);
		RBLXMATH_CTOR_DECL vec(T x, T y, T z, T w);

		// -- Conversion scalar constructors --

		template<typename U, qualifier P>
		RBLXMATH_CTOR_DECL explicit vec(vec<1, U, P> const& v);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(X _x, Y _y, Z _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, Y _y, Z _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(X _x, vec<1, Y, Q> const& _y, Z _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, vec<1, Y, Q> const& _y, Z _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(X _x, Y _y, vec<1, Z, Q> const& _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, Y _y, vec<1, Z, Q> const& _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(X _x, vec<1, Y, Q> const& _y, vec<1, Z, Q> const& _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, vec<1, Y, Q> const& _y, vec<1, Z, Q> const& _z, W _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, Y _y, Z _z, vec<1, W, Q> const& _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(X _x, vec<1, Y, Q> const& _y, Z _z, vec<1, W, Q> const& _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, vec<1, Y, Q> const& _y, Z _z, vec<1, W, Q> const& _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(X _x, Y _y, vec<1, Z, Q> const& _z, vec<1, W, Q> const& _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, Y _y, vec<1, Z, Q> const& _z, vec<1, W, Q> const& _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(X _x, vec<1, Y, Q> const& _y, vec<1, Z, Q> const& _z, vec<1, W, Q> const& _w);
		template<typename X, typename Y, typename Z, typename W>
		RBLXMATH_CTOR_DECL vec(vec<1, X, Q> const& _x, vec<1, Y, Q> const& _y, vec<1, Z, Q> const& _z, vec<1, W, Q> const& _w);

		// -- Conversion vector constructors --

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<2, A, P> const& _xy, B _z, C _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<2, A, P> const& _xy, vec<1, B, P> const& _z, C _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<2, A, P> const& _xy, B _z, vec<1, C, P> const& _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<2, A, P> const& _xy, vec<1, B, P> const& _z, vec<1, C, P> const& _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(A _x, vec<2, B, P> const& _yz, C _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<1, A, P> const& _x, vec<2, B, P> const& _yz, C _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(A _x, vec<2, B, P> const& _yz, vec<1, C, P> const& _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<1, A, P> const& _x, vec<2, B, P> const& _yz, vec<1, C, P> const& _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(A _x, B _y, vec<2, C, P> const& _zw);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<1, A, P> const& _x, B _y, vec<2, C, P> const& _zw);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(A _x, vec<1, B, P> const& _y, vec<2, C, P> const& _zw);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, typename C, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<1, A, P> const& _x, vec<1, B, P> const& _y, vec<2, C, P> const& _zw);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<3, A, P> const& _xyz, B _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<3, A, P> const& _xyz, vec<1, B, P> const& _w);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, qualifier P>
		RBLXMATH_CTOR_DECL vec(A _x, vec<3, B, P> const& _yzw);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<1, A, P> const& _x, vec<3, B, P> const& _yzw);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B, qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<2, A, P> const& _xy, vec<2, B, P> const& _zw);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT vec(vec<4, U, P> const& v);

		// -- Swizzle constructors --
#		if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
			template<int E0, int E1, int E2, int E3>
			RBLXMATH_FUNC_DISCARD_DECL vec(detail::_swizzle<4, T, Q, E0, E1, E2, E3> const& that)
			{
				*this = that();
			}

			template<int E0, int E1, int F0, int F1>
			RBLXMATH_FUNC_DISCARD_DECL vec(detail::_swizzle<2, T, Q, E0, E1, -1, -2> const& v, detail::_swizzle<2, T, Q, F0, F1, -1, -2> const& u)
			{
				*this = vec<4, T, Q>(v(), u());
			}

			template<int E0, int E1>
			RBLXMATH_FUNC_DISCARD_DECL vec(T const& x, T const& y, detail::_swizzle<2, T, Q, E0, E1, -1, -2> const& v)
			{
				*this = vec<4, T, Q>(x, y, v());
			}

			template<int E0, int E1>
			RBLXMATH_FUNC_DISCARD_DECL vec(T const& x, detail::_swizzle<2, T, Q, E0, E1, -1, -2> const& v, T const& w)
			{
				*this = vec<4, T, Q>(x, v(), w);
			}

			template<int E0, int E1>
			RBLXMATH_FUNC_DISCARD_DECL vec(detail::_swizzle<2, T, Q, E0, E1, -1, -2> const& v, T const& z, T const& w)
			{
				*this = vec<4, T, Q>(v(), z, w);
			}

			template<int E0, int E1, int E2>
			RBLXMATH_FUNC_DISCARD_DECL vec(detail::_swizzle<3, T, Q, E0, E1, E2, -1> const& v, T const& w)
			{
				*this = vec<4, T, Q>(v(), w);
			}

			template<int E0, int E1, int E2>
			RBLXMATH_FUNC_DISCARD_DECL vec(T const& x, detail::_swizzle<3, T, Q, E0, E1, E2, -1> const& v)
			{
				*this = vec<4, T, Q>(x, v());
			}
#		endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

		// -- Unary arithmetic operators --

		RBLXMATH_DEFAULTED_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator=(vec<4, T, Q> const& v) RBLXMATH_DEFAULT;

		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator+=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator+=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator+=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator-=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator-=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator-=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator*=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator*=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator*=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator/=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator/=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q>& operator/=(vec<4, U, Q> const& v);

		// -- Increment and decrement operators --

		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator++();
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator--();
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator++(int);
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator--(int);

		// -- Unary bit operators --

		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator%=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator%=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator%=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator&=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator&=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator&=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator|=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator|=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator|=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator^=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator^=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator^=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator<<=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator<<=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator<<=(vec<4, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator>>=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator>>=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> & operator>>=(vec<4, U, Q> const& v);
	};

	// -- Unary operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator+(vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator-(vec<4, T, Q> const& v);

	// -- Binary operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator+(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator+(vec<4, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator+(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator+(vec<1, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator+(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator-(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator-(vec<4, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator-(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator-(vec<1, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator-(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator*(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator*(vec<4, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator*(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator*(vec<1, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator*(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator/(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator/(vec<4, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator/(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator/(vec<1, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator/(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator%(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator%(vec<4, T, Q> const& v, vec<1, T, Q> const& scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator%(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator%(vec<1, T, Q> const& scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator%(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator&(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator&(vec<4, T, Q> const& v, vec<1, T, Q> const& scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator&(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator&(vec<1, T, Q> const& scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator&(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator|(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator|(vec<4, T, Q> const& v, vec<1, T, Q> const& scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator|(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator|(vec<1, T, Q> const& scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator|(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator^(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator^(vec<4, T, Q> const& v, vec<1, T, Q> const& scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator^(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator^(vec<1, T, Q> const& scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator^(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator<<(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator<<(vec<4, T, Q> const& v, vec<1, T, Q> const& scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator<<(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator<<(vec<1, T, Q> const& scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator<<(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator>>(vec<4, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator>>(vec<4, T, Q> const& v, vec<1, T, Q> const& scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator>>(T scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator>>(vec<1, T, Q> const& scalar, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator>>(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator~(vec<4, T, Q> const& v);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR bool operator==(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR bool operator!=(vec<4, T, Q> const& v1, vec<4, T, Q> const& v2);

	template<qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, bool, Q> operator&&(vec<4, bool, Q> const& v1, vec<4, bool, Q> const& v2);

	template<qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, bool, Q> operator||(vec<4, bool, Q> const& v1, vec<4, bool, Q> const& v2);
}//namespace rblxmath

#ifndef RBLXMATH_EXTERNAL_TEMPLATE
#include "type_vec4.inl"
#endif//RBLXMATH_EXTERNAL_TEMPLATE
