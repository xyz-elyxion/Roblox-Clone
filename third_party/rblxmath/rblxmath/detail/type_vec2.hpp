/// @ref core
/// @file rblxmath/detail/type_vec2.hpp

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
	struct vec<2, T, Q>
	{
		// -- Implementation detail --

		typedef T value_type;
		typedef vec<2, T, Q> type;
		typedef vec<2, bool, Q> bool_type;

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
			T x, y;
#			if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
			RBLXMATH_SWIZZLE_GEN_VEC_FROM_VEC2_COMP(T, Q, x, y)
#			endif//RBLXMATH_CONFIG_SWIZZLE
#		elif RBLXMATH_CONFIG_ANONYMOUS_STRUCT == RBLXMATH_ENABLE
			union
			{
				struct{ T x, y; };
				struct{ T r, g; };
				struct{ T s, t; };

				typename detail::storage<2, T, detail::is_aligned<Q>::value>::type data;

#				if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
					RBLXMATH_SWIZZLE2_2_MEMBERS(T, Q, x, y)
					RBLXMATH_SWIZZLE2_2_MEMBERS(T, Q, r, g)
					RBLXMATH_SWIZZLE2_2_MEMBERS(T, Q, s, t)
					RBLXMATH_SWIZZLE2_3_MEMBERS(T, Q, x, y)
					RBLXMATH_SWIZZLE2_3_MEMBERS(T, Q, r, g)
					RBLXMATH_SWIZZLE2_3_MEMBERS(T, Q, s, t)
					RBLXMATH_SWIZZLE2_4_MEMBERS(T, Q, x, y)
					RBLXMATH_SWIZZLE2_4_MEMBERS(T, Q, r, g)
					RBLXMATH_SWIZZLE2_4_MEMBERS(T, Q, s, t)
#				endif
			};
#		else
			union {T x, r, s;};
			union {T y, g, t;};

#			if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_FUNCTION
				RBLXMATH_SWIZZLE_GEN_VEC_FROM_VEC2(T, Q)
#			endif//RBLXMATH_CONFIG_SWIZZLE
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

		/// Return the count of components of the vector
		typedef length_t length_type;
		RBLXMATH_FUNC_DECL static RBLXMATH_CONSTEXPR length_type length(){return 2;}

		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T& operator[](length_type i);
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T const& operator[](length_type i) const;

		// -- Implicit basic constructors --

		RBLXMATH_DEFAULTED_DEFAULT_CTOR_DECL RBLXMATH_CONSTEXPR vec() RBLXMATH_DEFAULT_CTOR;
		RBLXMATH_DEFAULTED_FUNC_DECL RBLXMATH_CONSTEXPR vec(vec const& v) RBLXMATH_DEFAULT;
		template<qualifier P>
		RBLXMATH_CTOR_DECL vec(vec<2, T, P> const& v);

		// -- Explicit basic constructors --

		RBLXMATH_CTOR_DECL explicit vec(T scalar);
		RBLXMATH_CTOR_DECL vec(T x, T y);

		// -- Conversion constructors --

		template<typename U, qualifier P>
		RBLXMATH_CTOR_DECL explicit vec(vec<1, U, P> const& v);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename A, typename B>
		RBLXMATH_CTOR_DECL vec(A x, B y);
		template<typename A, typename B>
		RBLXMATH_CTOR_DECL vec(vec<1, A, Q> const& x, B y);
		template<typename A, typename B>
		RBLXMATH_CTOR_DECL vec(A x, vec<1, B, Q> const& y);
		template<typename A, typename B>
		RBLXMATH_CTOR_DECL vec(vec<1, A, Q> const& x, vec<1, B, Q> const& y);

		// -- Conversion vector constructors --

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT vec(vec<3, U, P> const& v);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT vec(vec<4, U, P> const& v);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT vec(vec<2, U, P> const& v);

		// -- Swizzle constructors --
#		if RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR
			template<int E0, int E1>
			RBLXMATH_FUNC_DISCARD_DECL vec(detail::_swizzle<2, T, Q, E0, E1,-1,-2> const& that)
			{
				*this = that();
			}
#		endif//RBLXMATH_CONFIG_SWIZZLE == RBLXMATH_SWIZZLE_OPERATOR

		// -- Unary arithmetic operators --

		RBLXMATH_DEFAULTED_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator=(vec const& v) RBLXMATH_DEFAULT;

		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator+=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator+=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator+=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator-=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator-=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator-=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator*=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator*=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator*=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator/=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator/=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator/=(vec<2, U, Q> const& v);

		// -- Increment and decrement operators --

		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator++();
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator--();
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator++(int);
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator--(int);

		// -- Unary bit operators --

		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator%=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator%=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator%=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator&=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator&=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator&=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator|=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator|=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator|=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator^=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator^=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator^=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator<<=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator<<=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator<<=(vec<2, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator>>=(U scalar);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator>>=(vec<1, U, Q> const& v);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> & operator>>=(vec<2, U, Q> const& v);
	};

	// -- Unary operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v);

	// -- Binary operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator+(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator+(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator+(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator-(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator-(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator-(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator*(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator*(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator*(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator/(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator/(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator/(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator%(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator%(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator%(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator&(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator&(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator&(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator|(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator|(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator|(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator^(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator^(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator^(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator<<(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator<<(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator<<(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator>>(T scalar, vec<2, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator>>(vec<1, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator>>(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, T, Q> operator~(vec<2, T, Q> const& v);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR bool operator==(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR bool operator!=(vec<2, T, Q> const& v1, vec<2, T, Q> const& v2);

	template<qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, bool, Q> operator&&(vec<2, bool, Q> const& v1, vec<2, bool, Q> const& v2);

	template<qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<2, bool, Q> operator||(vec<2, bool, Q> const& v1, vec<2, bool, Q> const& v2);
}//namespace rblxmath

#ifndef RBLXMATH_EXTERNAL_TEMPLATE
#include "type_vec2.inl"
#endif//RBLXMATH_EXTERNAL_TEMPLATE
