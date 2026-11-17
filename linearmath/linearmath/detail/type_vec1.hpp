/// @ref core
/// @file linearmath/detail/type_vec1.hpp

#pragma once

#include "qualifier.hpp"
#if LINEARMATH_CONFIG_SWIZZLE == LINEARMATH_SWIZZLE_OPERATOR
#	include "_swizzle.hpp"
#elif LINEARMATH_CONFIG_SWIZZLE == LINEARMATH_SWIZZLE_FUNCTION
#	include "_swizzle_func.hpp"
#endif
#include <cstddef>

namespace linearmath
{
	template<typename T, qualifier Q>
	struct vec<1, T, Q>
	{
		// -- Implementation detail --

		typedef T value_type;
		typedef vec<1, T, Q> type;
		typedef vec<1, bool, Q> bool_type;

		// -- Data --

#		if LINEARMATH_SILENT_WARNINGS == LINEARMATH_ENABLE
#			if LINEARMATH_COMPILER & LINEARMATH_COMPILER_GCC
#				pragma GCC diagnostic push
#				pragma GCC diagnostic ignored "-Wpedantic"
#			elif LINEARMATH_COMPILER & LINEARMATH_COMPILER_CLANG
#				pragma clang diagnostic push
#				pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#				pragma clang diagnostic ignored "-Wnested-anon-types"
#			elif LINEARMATH_COMPILER & LINEARMATH_COMPILER_VC
#				pragma warning(push)
#				pragma warning(disable: 4201)  // nonstandard extension used : nameless struct/union
#			endif
#		endif

#		if LINEARMATH_CONFIG_XYZW_ONLY
			T x;
#		elif LINEARMATH_CONFIG_ANONYMOUS_STRUCT == LINEARMATH_ENABLE
			union
			{
				T x;
				T r;
				T s;

				typename detail::storage<1, T, detail::is_aligned<Q>::value>::type data;
/*
#				if LINEARMATH_CONFIG_SWIZZLE == LINEARMATH_SWIZZLE_OPERATOR
					_GLM_SWIZZLE1_2_MEMBERS(T, Q, x)
					_GLM_SWIZZLE1_2_MEMBERS(T, Q, r)
					_GLM_SWIZZLE1_2_MEMBERS(T, Q, s)
					_GLM_SWIZZLE1_3_MEMBERS(T, Q, x)
					_GLM_SWIZZLE1_3_MEMBERS(T, Q, r)
					_GLM_SWIZZLE1_3_MEMBERS(T, Q, s)
					_GLM_SWIZZLE1_4_MEMBERS(T, Q, x)
					_GLM_SWIZZLE1_4_MEMBERS(T, Q, r)
					_GLM_SWIZZLE1_4_MEMBERS(T, Q, s)
#				endif
*/
			};
#		else
			union {T x, r, s;};
/*
#			if LINEARMATH_CONFIG_SWIZZLE == LINEARMATH_SWIZZLE_FUNCTION
				LINEARMATH_SWIZZLE_GEN_VEC_FROM_VEC1(T, Q)
#			endif
*/
#		endif

#		if LINEARMATH_SILENT_WARNINGS == LINEARMATH_ENABLE
#			if LINEARMATH_COMPILER & LINEARMATH_COMPILER_CLANG
#				pragma clang diagnostic pop
#			elif LINEARMATH_COMPILER & LINEARMATH_COMPILER_GCC
#				pragma GCC diagnostic pop
#			elif LINEARMATH_COMPILER & LINEARMATH_COMPILER_VC
#				pragma warning(pop)
#			endif
#		endif

		// -- Component accesses --

		/// Return the count of components of the vector
		typedef length_t length_type;
		LINEARMATH_FUNC_DECL static LINEARMATH_CONSTEXPR length_type length(){return 1;}

		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR T & operator[](length_type i);
		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR T const& operator[](length_type i) const;

		// -- Implicit basic constructors --

		LINEARMATH_DEFAULTED_DEFAULT_CTOR_DECL LINEARMATH_CONSTEXPR vec() LINEARMATH_DEFAULT_CTOR;
		LINEARMATH_DEFAULTED_FUNC_DECL LINEARMATH_CONSTEXPR vec(vec const& v) LINEARMATH_DEFAULT;
		template<qualifier P>
		LINEARMATH_CTOR_DECL vec(vec<1, T, P> const& v);

		// -- Explicit basic constructors --

		LINEARMATH_CTOR_DECL explicit vec(T scalar);

		// -- Conversion vector constructors --

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT vec(vec<2, U, P> const& v);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT vec(vec<3, U, P> const& v);
		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT vec(vec<4, U, P> const& v);

		/// Explicit conversions (From section 5.4.1 Conversion and scalar constructors of GLSL 1.30.08 specification)
		template<typename U, qualifier P>
		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT vec(vec<1, U, P> const& v);

		// -- Swizzle constructors --
/*
#		if LINEARMATH_CONFIG_SWIZZLE == LINEARMATH_SWIZZLE_OPERATOR
			template<int E0>
			LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec(detail::_swizzle<1, T, Q, E0, -1,-2,-3> const& that)
			{
				*this = that();
			}
#		endif//LINEARMATH_CONFIG_SWIZZLE == LINEARMATH_SWIZZLE_OPERATOR
*/
		// -- Unary arithmetic operators --

		LINEARMATH_DEFAULTED_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator=(vec const& v) LINEARMATH_DEFAULT;

		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator+=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator+=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator-=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator-=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator*=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator*=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator/=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator/=(vec<1, U, Q> const& v);

		// -- Increment and decrement operators --

		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator++();
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator--();
		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator++(int);
		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator--(int);

		// -- Unary bit operators --

		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator%=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator%=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator&=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator&=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator|=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator|=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator^=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator^=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator<<=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator<<=(vec<1, U, Q> const& v);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator>>=(U scalar);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> & operator>>=(vec<1, U, Q> const& v);
	};

	// -- Unary operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator+(vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator-(vec<1, T, Q> const& v);

	// -- Binary operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator+(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator+(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator+(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator-(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator-(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator-(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator*(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator*(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator*(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator/(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator/(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator/(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator%(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator%(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator%(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator&(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator&(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator&(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator|(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator|(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator|(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator^(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator^(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator^(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator<<(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator<<(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator<<(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator>>(vec<1, T, Q> const& v, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator>>(T scalar, vec<1, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator>>(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, T, Q> operator~(vec<1, T, Q> const& v);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR bool operator==(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR bool operator!=(vec<1, T, Q> const& v1, vec<1, T, Q> const& v2);

	template<qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, bool, Q> operator&&(vec<1, bool, Q> const& v1, vec<1, bool, Q> const& v2);

	template<qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR vec<1, bool, Q> operator||(vec<1, bool, Q> const& v1, vec<1, bool, Q> const& v2);
}//namespace linearmath

#ifndef LINEARMATH_EXTERNAL_TEMPLATE
#include "type_vec1.inl"
#endif//LINEARMATH_EXTERNAL_TEMPLATE
