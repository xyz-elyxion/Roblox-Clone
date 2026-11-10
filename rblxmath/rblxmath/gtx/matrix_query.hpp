/// @ref gtx_matrix_query
/// @file rblxmath/gtx/matrix_query.hpp
///
/// @see core (dependence)
/// @see gtx_vector_query (dependence)
///
/// @defgroup gtx_matrix_query RBLXMATH_GTX_matrix_query
/// @ingroup gtx
///
/// Include <rblxmath/gtx/matrix_query.hpp> to use the features of this extension.
///
/// Query to evaluate matrix properties

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include "../gtx/vector_query.hpp"
#include <limits>

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_matrix_query is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_matrix_query extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_matrix_query
	/// @{

	/// Return whether a matrix a null matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNull(mat<2, 2, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix a null matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNull(mat<3, 3, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a null matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNull(mat<4, 4, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is an identity matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<length_t C, length_t R, typename T, qualifier Q, template<length_t, length_t, typename, qualifier> class matType>
	RBLXMATH_FUNC_DECL bool isIdentity(matType<C, R, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a normalized matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNormalized(mat<2, 2, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a normalized matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNormalized(mat<3, 3, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is a normalized matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNormalized(mat<4, 4, T, Q> const& m, T const& epsilon);

	/// Return whether a matrix is an orthonormalized matrix.
	/// From RBLXMATH_GTX_matrix_query extension.
	template<length_t C, length_t R, typename T, qualifier Q, template<length_t, length_t, typename, qualifier> class matType>
	RBLXMATH_FUNC_DECL bool isOrthogonal(matType<C, R, T, Q> const& m, T const& epsilon);

	/// @}
}//namespace rblxmath

#include "matrix_query.inl"
