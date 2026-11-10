/// @ref gtx_vector_query
/// @file rblxmath/gtx/vector_query.hpp
///
/// @see core (dependence)
///
/// @defgroup gtx_vector_query RBLXMATH_GTX_vector_query
/// @ingroup gtx
///
/// Include <rblxmath/gtx/vector_query.hpp> to use the features of this extension.
///
/// Query information of vector types

#pragma once

// Dependency:
#include "../rblxmath.hpp"
#include <cfloat>
#include <limits>

#ifndef RBLXMATH_ENABLE_EXPERIMENTAL
#	error "RBLXMATH: RBLXMATH_GTX_vector_query is an experimental extension and may change in the future. Use #define RBLXMATH_ENABLE_EXPERIMENTAL before including it, if you really want to use it."
#elif RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_GTX_vector_query extension included")
#endif

namespace rblxmath
{
	/// @addtogroup gtx_vector_query
	/// @{

	//! Check whether two vectors are collinears.
	/// @see gtx_vector_query extensions.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool areCollinear(vec<L, T, Q> const& v0, vec<L, T, Q> const& v1, T const& epsilon);

	//! Check whether two vectors are orthogonals.
	/// @see gtx_vector_query extensions.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool areOrthogonal(vec<L, T, Q> const& v0, vec<L, T, Q> const& v1, T const& epsilon);

	//! Check whether a vector is normalized.
	/// @see gtx_vector_query extensions.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNormalized(vec<L, T, Q> const& v, T const& epsilon);

	//! Check whether a vector is null.
	/// @see gtx_vector_query extensions.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool isNull(vec<L, T, Q> const& v, T const& epsilon);

	//! Check whether a each component of a vector is null.
	/// @see gtx_vector_query extensions.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<L, bool, Q> isCompNull(vec<L, T, Q> const& v, T const& epsilon);

	//! Check whether two vectors are orthonormal.
	/// @see gtx_vector_query extensions.
	template<length_t L, typename T, qualifier Q>
	RBLXMATH_FUNC_DECL bool areOrthonormal(vec<L, T, Q> const& v0, vec<L, T, Q> const& v1, T const& epsilon);

	/// @}
}// namespace rblxmath

#include "vector_query.inl"
