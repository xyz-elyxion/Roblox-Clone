/// @ref core
/// @file linearmath/detail/type_mat4x3.hpp

#pragma once

#include "type_vec3.hpp"
#include "type_vec4.hpp"
#include <limits>
#include <cstddef>

namespace linearmath
{
	template<typename T, qualifier Q>
	struct mat<4, 3, T, Q>
	{
		typedef vec<3, T, Q> col_type;
		typedef vec<4, T, Q> row_type;
		typedef mat<4, 3, T, Q> type;
		typedef mat<3, 4, T, Q> transpose_type;
		typedef T value_type;

	private:
		col_type value[4];

	public:
		// -- Accesses --

		typedef length_t length_type;
		LINEARMATH_FUNC_DECL static LINEARMATH_CONSTEXPR length_type length() { return 4; }

		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR col_type & operator[](length_type i) LINEARMATH_NOEXCEPT;
		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR col_type const& operator[](length_type i) const LINEARMATH_NOEXCEPT;

		// -- Constructors --

		LINEARMATH_DEFAULTED_DEFAULT_CTOR_DECL LINEARMATH_CONSTEXPR mat() LINEARMATH_DEFAULT_CTOR;
		template<qualifier P>
		LINEARMATH_CTOR_DECL mat(mat<4, 3, T, P> const& m);

		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT mat(T s);
		LINEARMATH_CTOR_DECL mat(
			T const& x0, T const& y0, T const& z0,
			T const& x1, T const& y1, T const& z1,
			T const& x2, T const& y2, T const& z2,
			T const& x3, T const& y3, T const& z3);
		LINEARMATH_CTOR_DECL mat(
			col_type const& v0,
			col_type const& v1,
			col_type const& v2,
			col_type const& v3);

		// -- Conversions --

		template<
			typename X1, typename Y1, typename Z1,
			typename X2, typename Y2, typename Z2,
			typename X3, typename Y3, typename Z3,
			typename X4, typename Y4, typename Z4>
		LINEARMATH_CTOR_DECL mat(
			X1 const& x1, Y1 const& y1, Z1 const& z1,
			X2 const& x2, Y2 const& y2, Z2 const& z2,
			X3 const& x3, Y3 const& y3, Z3 const& z3,
			X4 const& x4, Y4 const& y4, Z4 const& z4);

		template<typename V1, typename V2, typename V3, typename V4>
		LINEARMATH_CTOR_DECL mat(
			vec<3, V1, Q> const& v1,
			vec<3, V2, Q> const& v2,
			vec<3, V3, Q> const& v3,
			vec<3, V4, Q> const& v4);

		// -- Matrix conversions --

		template<typename U, qualifier P>
		LINEARMATH_CTOR_DECL mat(mat<4, 3, U, P> const& m);

		LINEARMATH_CTOR_DECL mat(mat<2, 2, T, Q> const& x);
		LINEARMATH_CTOR_DECL mat(mat<3, 3, T, Q> const& x);
		LINEARMATH_CTOR_DECL mat(mat<4, 4, T, Q> const& x);
		LINEARMATH_CTOR_DECL mat(mat<2, 3, T, Q> const& x);
		LINEARMATH_CTOR_DECL mat(mat<3, 2, T, Q> const& x);
		LINEARMATH_CTOR_DECL mat(mat<2, 4, T, Q> const& x);
		LINEARMATH_CTOR_DECL mat(mat<4, 2, T, Q> const& x);
		LINEARMATH_CTOR_DECL mat(mat<3, 4, T, Q> const& x);

		// -- Unary arithmetic operators --

		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> & operator=(mat<4, 3, U, Q> const& m);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> & operator+=(U s);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> & operator+=(mat<4, 3, U, Q> const& m);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> & operator-=(U s);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> & operator-=(mat<4, 3, U, Q> const& m);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> & operator*=(U s);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> & operator/=(U s);

		// -- Increment and decrement operators --

		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q>& operator++();
		LINEARMATH_FUNC_DISCARD_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q>& operator--();
		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator++(int);
		LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator--(int);
	};

	// -- Unary operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator+(mat<4, 3, T, Q> const& m);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator-(mat<4, 3, T, Q> const& m);

	// -- Binary operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator+(mat<4, 3, T, Q> const& m, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator+(mat<4, 3, T, Q> const& m1, mat<4, 3, T, Q> const& m2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator-(mat<4, 3, T, Q> const& m, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator-(mat<4, 3, T, Q> const& m1, mat<4, 3, T, Q> const& m2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator*(mat<4, 3, T, Q> const& m, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator*(T scalar, mat<4, 3, T, Q> const& m);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR typename mat<4, 3, T, Q>::col_type operator*(mat<4, 3, T, Q> const& m, typename mat<4, 3, T, Q>::row_type const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR typename mat<4, 3, T, Q>::row_type operator*(typename mat<4, 3, T, Q>::col_type const& v, mat<4, 3, T, Q> const& m);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<2, 3, T, Q> operator*(mat<4, 3, T, Q> const& m1, mat<2, 4, T, Q> const& m2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<3, 3, T, Q> operator*(mat<4, 3, T, Q> const& m1,	mat<3, 4, T, Q> const& m2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator*(mat<4, 3, T, Q> const& m1, mat<4, 4, T, Q> const& m2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator/(mat<4, 3, T, Q> const& m, T scalar);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR mat<4, 3, T, Q> operator/(T scalar, mat<4, 3, T, Q> const& m);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR bool operator==(mat<4, 3, T, Q> const& m1, mat<4, 3, T, Q> const& m2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL LINEARMATH_CONSTEXPR bool operator!=(mat<4, 3, T, Q> const& m1, mat<4, 3, T, Q> const& m2);
}//namespace linearmath

#ifndef LINEARMATH_EXTERNAL_TEMPLATE
#include "type_mat4x3.inl"
#endif //LINEARMATH_EXTERNAL_TEMPLATE
