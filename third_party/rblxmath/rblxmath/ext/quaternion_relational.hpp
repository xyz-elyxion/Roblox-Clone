/// @ref ext_quaternion_relational
/// @file rblxmath/ext/quaternion_relational.hpp
///
/// @defgroup ext_quaternion_relational RBLXMATH_EXT_quaternion_relational
/// @ingroup ext
///
/// Exposes comparison functions for quaternion types that take a user defined epsilon values.
///
/// Include <rblxmath/ext/quaternion_relational.hpp> to use the features of this extension.
///
/// @see core_vector_relational
/// @see ext_vector_relational
/// @see ext_matrix_relational
/// @see ext_quaternion_float
/// @see ext_quaternion_double

#pragma once

// Dependency:
#include "../vector_relational.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_quaternion_relational extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_quaternion_relational
	/// @{

	/// Returns the component-wise comparison of result x == y.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<4, bool, Q> equal(qua<T, Q> const& x, qua<T, Q> const& y);

	/// Returns the component-wise comparison of |x - y| < epsilon.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<4, bool, Q> equal(qua<T, Q> const& x, qua<T, Q> const& y, T epsilon);

	/// Returns the component-wise comparison of result x != y.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<4, bool, Q> notEqual(qua<T, Q> const& x, qua<T, Q> const& y);

	/// Returns the component-wise comparison of |x - y| >= epsilon.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL vec<4, bool, Q> notEqual(qua<T, Q> const& x, qua<T, Q> const& y, T epsilon);

	/// @}
} //namespace rblxmath

#include "quaternion_relational.inl"
