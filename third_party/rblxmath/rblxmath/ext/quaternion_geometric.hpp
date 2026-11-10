/// @ref ext_quaternion_geometric
/// @file rblxmath/ext/quaternion_geometric.hpp
///
/// @defgroup ext_quaternion_geometric RBLXMATH_EXT_quaternion_geometric
/// @ingroup ext
///
/// Provides geometric functions for quaternion types
///
/// Include <rblxmath/ext/quaternion_geometric.hpp> to use the features of this extension.
///
/// @see core_func_geometric
/// @see ext_quaternion_float
/// @see ext_quaternion_double

#pragma once

// Dependency:
#include "../geometric.hpp"
#include "../exponential.hpp"
#include "../ext/vector_relational.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_quaternion_geometric extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_quaternion_geometric
	/// @{

	/// Returns the norm of a quaternions
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	///
	/// @see ext_quaternion_geometric
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL T length(qua<T, Q> const& q);

	/// Returns the normalized quaternion.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	///
	/// @see ext_quaternion_geometric
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL qua<T, Q> normalize(qua<T, Q> const& q);

	/// Returns dot product of q1 and q2, i.e., q1[0] * q2[0] + q1[1] * q2[1] + ...
	///
	/// @tparam T Floating-point scalar types.
	/// @tparam Q Value from qualifier enum
	///
	/// @see ext_quaternion_geometric
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_DECL RBLXMATH_CONSTEXPR T dot(qua<T, Q> const& x, qua<T, Q> const& y);

	/// Compute a cross product.
	///
	/// @tparam T Floating-point scalar types
	/// @tparam Q Value from qualifier enum
	///
	/// @see ext_quaternion_geometric
	template<typename T, qualifier Q>
	RBLXMATH_FUNC_QUALIFIER RBLXMATH_CONSTEXPR qua<T, Q> cross(qua<T, Q> const& q1, qua<T, Q> const& q2);

	/// @}
} //namespace rblxmath

#include "quaternion_geometric.inl"
