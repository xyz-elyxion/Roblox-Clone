/// @ref gtx_dual_quaternion
/// @file linearmath/gtx/dual_quaternion.hpp
/// @author Maksim Vorobiev (msomeone@gmail.com)
///
/// @see core (dependence)
/// @see gtc_constants (dependence)
/// @see gtc_quaternion (dependence)
///
/// @defgroup gtx_dual_quaternion LINEARMATH_GTX_dual_quaternion
/// @ingroup gtx
///
/// Include <linearmath/gtx/dual_quaternion.hpp> to use the features of this extension.
///
/// Defines a templated dual-quaternion type and several dual-quaternion operations.

#pragma once

// Dependency:
#include "../linearmath.hpp"
#include "../gtc/constants.hpp"
#include "../gtc/quaternion.hpp"

#ifndef LINEARMATH_ENABLE_EXPERIMENTAL
#	error "LINEARMATH: LINEARMATH_GTX_dual_quaternion is an experimental extension and may change in the future. Use #define LINEARMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_GTX_dual_quaternion extension included")
#endif

namespace linearmath
{
	/// @addtogroup gtx_dual_quaternion
	/// @{

	template<typename T, qualifier Q = defaultp>
	struct tdualquat
	{
		// -- Implementation detail --

		typedef T value_type;
		typedef qua<T, Q> part_type;

		// -- Data --

		qua<T, Q> real, dual;

		// -- Component accesses --

		typedef length_t length_type;
		/// Return the count of components of a dual quaternion
		LINEARMATH_FUNC_DECL static LINEARMATH_CONSTEXPR length_type length(){return 2;}

		LINEARMATH_FUNC_DECL part_type & operator[](length_type i);
		LINEARMATH_FUNC_DECL part_type const& operator[](length_type i) const;

		// -- Implicit basic constructors --

		LINEARMATH_DEFAULTED_FUNC_DECL LINEARMATH_CONSTEXPR tdualquat() LINEARMATH_DEFAULT;
		LINEARMATH_DEFAULTED_FUNC_DECL LINEARMATH_CONSTEXPR tdualquat(tdualquat<T, Q> const& d) LINEARMATH_DEFAULT;
		template<qualifier P>
		LINEARMATH_CTOR_DECL tdualquat(tdualquat<T, P> const& d);

		// -- Explicit basic constructors --

		LINEARMATH_CTOR_DECL tdualquat(qua<T, Q> const& real);
		LINEARMATH_CTOR_DECL tdualquat(qua<T, Q> const& orientation, vec<3, T, Q> const& translation);
		LINEARMATH_CTOR_DECL tdualquat(qua<T, Q> const& real, qua<T, Q> const& dual);

		// -- Conversion constructors --

		template<typename U, qualifier P>
		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT tdualquat(tdualquat<U, P> const& q);

		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT tdualquat(mat<2, 4, T, Q> const& holder_mat);
		LINEARMATH_CTOR_DECL LINEARMATH_EXPLICIT tdualquat(mat<3, 4, T, Q> const& aug_mat);

		// -- Unary arithmetic operators --

		LINEARMATH_DEFAULTED_FUNC_DECL tdualquat<T, Q> & operator=(tdualquat<T, Q> const& m) LINEARMATH_DEFAULT;

		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL tdualquat<T, Q> & operator=(tdualquat<U, Q> const& m);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL tdualquat<T, Q> & operator*=(U s);
		template<typename U>
		LINEARMATH_FUNC_DISCARD_DECL tdualquat<T, Q> & operator/=(U s);
	};

	// -- Unary bit operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> operator+(tdualquat<T, Q> const& q);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> operator-(tdualquat<T, Q> const& q);

	// -- Binary operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> operator+(tdualquat<T, Q> const& q, tdualquat<T, Q> const& p);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> operator*(tdualquat<T, Q> const& q, tdualquat<T, Q> const& p);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<3, T, Q> operator*(tdualquat<T, Q> const& q, vec<3, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<3, T, Q> operator*(vec<3, T, Q> const& v, tdualquat<T, Q> const& q);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<4, T, Q> operator*(tdualquat<T, Q> const& q, vec<4, T, Q> const& v);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL vec<4, T, Q> operator*(vec<4, T, Q> const& v, tdualquat<T, Q> const& q);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> operator*(tdualquat<T, Q> const& q, T const& s);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> operator*(T const& s, tdualquat<T, Q> const& q);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> operator/(tdualquat<T, Q> const& q, T const& s);

	// -- Boolean operators --

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL bool operator==(tdualquat<T, Q> const& q1, tdualquat<T, Q> const& q2);

	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL bool operator!=(tdualquat<T, Q> const& q1, tdualquat<T, Q> const& q2);

	/// Creates an identity dual quaternion.
	///
	/// @see gtx_dual_quaternion
	template <typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> dual_quat_identity();

	/// Returns the normalized quaternion.
	///
	/// @see gtx_dual_quaternion
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> normalize(tdualquat<T, Q> const& q);

	/// Returns the linear interpolation of two dual quaternion.
	///
	/// @see gtc_dual_quaternion
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> lerp(tdualquat<T, Q> const& x, tdualquat<T, Q> const& y, T const& a);

	/// Returns the q inverse.
	///
	/// @see gtx_dual_quaternion
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> inverse(tdualquat<T, Q> const& q);

	/// Converts a quaternion to a 2 * 4 matrix.
	///
	/// @see gtx_dual_quaternion
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL mat<2, 4, T, Q> mat2x4_cast(tdualquat<T, Q> const& x);

	/// Converts a quaternion to a 3 * 4 matrix.
	///
	/// @see gtx_dual_quaternion
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL mat<3, 4, T, Q> mat3x4_cast(tdualquat<T, Q> const& x);

	/// Converts a 2 * 4 matrix (matrix which holds real and dual parts) to a quaternion.
	///
	/// @see gtx_dual_quaternion
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> dualquat_cast(mat<2, 4, T, Q> const& x);

	/// Converts a 3 * 4 matrix (augmented matrix rotation + translation) to a quaternion.
	///
	/// @see gtx_dual_quaternion
	template<typename T, qualifier Q>
	LINEARMATH_FUNC_DECL tdualquat<T, Q> dualquat_cast(mat<3, 4, T, Q> const& x);


	/// Dual-quaternion of low single-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<float, lowp>		lowp_dualquat;

	/// Dual-quaternion of medium single-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<float, mediump>	mediump_dualquat;

	/// Dual-quaternion of high single-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<float, highp>		highp_dualquat;


	/// Dual-quaternion of low single-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<float, lowp>		lowp_fdualquat;

	/// Dual-quaternion of medium single-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<float, mediump>	mediump_fdualquat;

	/// Dual-quaternion of high single-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<float, highp>		highp_fdualquat;


	/// Dual-quaternion of low double-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<double, lowp>		lowp_ddualquat;

	/// Dual-quaternion of medium double-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<double, mediump>	mediump_ddualquat;

	/// Dual-quaternion of high double-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef tdualquat<double, highp>	highp_ddualquat;


#if(!defined(LINEARMATH_PRECISION_HIGHP_FLOAT) && !defined(LINEARMATH_PRECISION_MEDIUMP_FLOAT) && !defined(LINEARMATH_PRECISION_LOWP_FLOAT))
	/// Dual-quaternion of floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef highp_fdualquat			dualquat;

	/// Dual-quaternion of single-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef highp_fdualquat			fdualquat;
#elif(defined(LINEARMATH_PRECISION_HIGHP_FLOAT) && !defined(LINEARMATH_PRECISION_MEDIUMP_FLOAT) && !defined(LINEARMATH_PRECISION_LOWP_FLOAT))
	typedef highp_fdualquat			dualquat;
	typedef highp_fdualquat			fdualquat;
#elif(!defined(LINEARMATH_PRECISION_HIGHP_FLOAT) && defined(LINEARMATH_PRECISION_MEDIUMP_FLOAT) && !defined(LINEARMATH_PRECISION_LOWP_FLOAT))
	typedef mediump_fdualquat		dualquat;
	typedef mediump_fdualquat		fdualquat;
#elif(!defined(LINEARMATH_PRECISION_HIGHP_FLOAT) && !defined(LINEARMATH_PRECISION_MEDIUMP_FLOAT) && defined(LINEARMATH_PRECISION_LOWP_FLOAT))
	typedef lowp_fdualquat			dualquat;
	typedef lowp_fdualquat			fdualquat;
#else
#	error "LINEARMATH error: multiple default precision requested for single-precision floating-point types"
#endif


#if(!defined(LINEARMATH_PRECISION_HIGHP_DOUBLE) && !defined(LINEARMATH_PRECISION_MEDIUMP_DOUBLE) && !defined(LINEARMATH_PRECISION_LOWP_DOUBLE))
	/// Dual-quaternion of default double-qualifier floating-point numbers.
	///
	/// @see gtx_dual_quaternion
	typedef highp_ddualquat			ddualquat;
#elif(defined(LINEARMATH_PRECISION_HIGHP_DOUBLE) && !defined(LINEARMATH_PRECISION_MEDIUMP_DOUBLE) && !defined(LINEARMATH_PRECISION_LOWP_DOUBLE))
	typedef highp_ddualquat			ddualquat;
#elif(!defined(LINEARMATH_PRECISION_HIGHP_DOUBLE) && defined(LINEARMATH_PRECISION_MEDIUMP_DOUBLE) && !defined(LINEARMATH_PRECISION_LOWP_DOUBLE))
	typedef mediump_ddualquat		ddualquat;
#elif(!defined(LINEARMATH_PRECISION_HIGHP_DOUBLE) && !defined(LINEARMATH_PRECISION_MEDIUMP_DOUBLE) && defined(LINEARMATH_PRECISION_LOWP_DOUBLE))
	typedef lowp_ddualquat			ddualquat;
#else
#	error "LINEARMATH error: Multiple default precision requested for double-precision floating-point types"
#endif

	/// @}
} //namespace linearmath

#include "dual_quaternion.inl"
