/// @ref core
/// @file rblxmath/detail/type_quat.hpp

#pragma once

// Dependency:
#include "../detail/type_mat3x3.hpp"
#include "../detail/type_mat4x4.hpp"
#include "../detail/type_vec3.hpp"
#include "../detail/type_vec4.hpp"
#include "../ext/vector_relational.hpp"
#include "../ext/quaternion_relational.hpp"
#include "../gtc/constants.hpp"
#include "../gtc/matrix_transform.hpp"

namespace rblxmath
{
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

	template<typename T, qualifier Q>
	struct qua
	{
		// -- Implementation detail --

		typedef qua<T, Q> type;
		typedef T value_type;

		// -- Data --

#		if RBLXMATH_LANG & RBLXMATH_LANG_CXXMS_FLAG
			union
			{
#				ifdef RBLXMATH_FORCE_QUAT_DATA_WXYZ
					struct { T w, x, y, z; };
#				else
					struct { T x, y, z, w; };
#				endif

				typename detail::storage<4, T, detail::is_aligned<Q>::value>::type data;
			};
#		else
#			ifdef RBLXMATH_FORCE_QUAT_DATA_WXYZ
				T w, x, y, z;
#			else
				T x, y, z, w;
#			endif
#		endif

		// -- Component accesses --

		typedef length_t length_type;

		/// Return the count of components of a quaternion
		RBLXMATH_FUNC_DECL static RBLXMATH_CONSTEXPR length_type length(){return 4;}

		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T & operator[](length_type i);
		RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T const& operator[](length_type i) const;

		// -- Implicit basic constructors --

		RBLXMATH_DEFAULTED_DEFAULT_CTOR_DECL RBLXMATH_CONSTEXPR qua() RBLXMATH_DEFAULT_CTOR;
		RBLXMATH_DEFAULTED_FUNC_DECL RBLXMATH_CONSTEXPR qua(qua<T, Q> const& q) RBLXMATH_DEFAULT;
		template<qualifier P>
		RBLXMATH_CTOR_DECL qua(qua<T, P> const& q);

		// -- Explicit basic constructors --

		RBLXMATH_CTOR_DECL qua(T s, vec<3, T, Q> const& v);

#		ifdef RBLXMATH_FORCE_QUAT_DATA_XYZW
		RBLXMATH_CTOR_DECL qua(T x, T y, T z, T w);
#		else
		RBLXMATH_CTOR_DECL qua(T w, T x, T y, T z);
#		endif

		RBLXMATH_FUNC_DECL static RBLXMATH_CONSTEXPR qua<T, Q> wxyz(T w, T x, T y, T z);

		// -- Conversion constructors --

		template<typename U, qualifier P>
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT qua(qua<U, P> const& q);

		/// Explicit conversion operators
#		if RBLXMATH_HAS_EXPLICIT_CONVERSION_OPERATORS
			RBLXMATH_FUNC_DECL explicit operator mat<3, 3, T, Q>() const;
			RBLXMATH_FUNC_DECL explicit operator mat<4, 4, T, Q>() const;
#		endif

		/// Create a quaternion from two normalized axis
		///
		/// @param u A first normalized axis
		/// @param v A second normalized axis
		/// @see gtc_quaternion
		/// @see http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors
		RBLXMATH_FUNC_DISCARD_DECL qua(vec<3, T, Q> const& u, vec<3, T, Q> const& v);

		/// Build a quaternion from euler angles (pitch, yaw, roll), in radians.
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT qua(vec<3, T, Q> const& eulerAngles);
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT qua(mat<3, 3, T, Q> const& q);
		RBLXMATH_CTOR_DECL RBLXMATH_EXPLICIT qua(mat<4, 4, T, Q> const& q);

		// -- Unary arithmetic operators --

		RBLXMATH_DEFAULTED_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q>& operator=(qua<T, Q> const& q) RBLXMATH_DEFAULT;

		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR qua<T, Q>& operator=(qua<U, Q> const& q);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR qua<T, Q>& operator+=(qua<U, Q> const& q);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR qua<T, Q>& operator-=(qua<U, Q> const& q);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR qua<T, Q>& operator*=(qua<U, Q> const& q);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR qua<T, Q>& operator*=(U s);
		template<typename U>
		RBLXMATH_FUNC_DISCARD_DECL RBLXMATH_CONSTEXPR qua<T, Q>& operator/=(U s);
	};

#		if RBLXMATH_SILENT_WARNINGS == RBLXMATH_ENABLE
#			if RBLXMATH_COMPILER & RBLXMATH_COMPILER_CLANG
#				pragma clang diagnostic pop
#			elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_GCC
#				pragma GCC diagnostic pop
#			elif RBLXMATH_COMPILER & RBLXMATH_COMPILER_VC
#				pragma warning(pop)
#			endif
#		endif

	// -- Unary bit operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator+(qua<T, Q> const& q);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator-(qua<T, Q> const& q);

	// -- Binary operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator+(qua<T, Q> const& q, qua<T, Q> const& p);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator-(qua<T, Q> const& q, qua<T, Q> const& p);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator*(qua<T, Q> const& q, qua<T, Q> const& p);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<3, T, Q> operator*(qua<T, Q> const& q, vec<3, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<3, T, Q> operator*(vec<3, T, Q> const& v, qua<T, Q> const& q);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator*(qua<T, Q> const& q, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR vec<4, T, Q> operator*(vec<4, T, Q> const& v, qua<T, Q> const& q);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator*(qua<T, Q> const& q, T const& s);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator*(T const& s, qua<T, Q> const& q);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR qua<T, Q> operator/(qua<T, Q> const& q, T const& s);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR bool operator==(qua<T, Q> const& q1, qua<T, Q> const& q2);

	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR bool operator!=(qua<T, Q> const& q1, qua<T, Q> const& q2);
} //namespace rblxmath

#ifndef RBLXMATH_EXTERNAL_TEMPLATE
#include "type_quat.inl"
#endif//RBLXMATH_EXTERNAL_TEMPLATE
