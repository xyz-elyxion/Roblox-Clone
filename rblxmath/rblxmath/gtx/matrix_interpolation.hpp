/// @ref gtx_matrix_interpolation
/// @file rblxmath/gtx/matrix_interpolation.hpp
/// @author Ghenadii Ursachi (the.asteroth@gmail.com)
///
/// @see core (dependence)
///
/// @defgroup gtx_matrix_interpolation RBLXMATH_GTX_matrix_interpolation
/// @ingroup gtx
///
/// Include <rblxmath/gtx/matrix_interpolation.hpp> to use the features of this extension.
///
/// Allows to directly interpolate two matrices.

#pragma once

// Dependency:
#include "../rblxmath.hpp"

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_matrix_interpolation is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_matrix_interpolation extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_matrix_interpolation
	/// @{

	/// Get the axis and angle of the rotation from a matrix.
	/// From RBLXMATH_GTX_matrix_interpolation extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DISCARD_DECL void axisAngle(
		mat<4, 4, T, Q> const& Mat, vec<3, T, Q> & Axis, T & Angle);

	/// Build a matrix from axis and angle.
	/// From RBLXMATH_GTX_matrix_interpolation extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL mat<4, 4, T, Q> axisAngleMatrix(
		vec<3, T, Q> const& Axis, T const Angle);

	/// Extracts the rotation part of a matrix.
	/// From RBLXMATH_GTX_matrix_interpolation extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL mat<4, 4, T, Q> extractMatrixRotation(
		mat<4, 4, T, Q> const& Mat);

	/// Build a interpolation of 4 * 4 matrixes.
	/// From RBLXMATH_GTX_matrix_interpolation extension.
	/// Warning! works only with rotation and/or translation matrixes, scale will generate unexpected results.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL mat<4, 4, T, Q> interpolate(
		mat<4, 4, T, Q> const& m1, mat<4, 4, T, Q> const& m2, T const Delta);

	/// @}
}//namespace rblxmath

#include "matrix_interpolation.inl"
