/// @ref ext_matrix_uint3x3
/// @file rblxmath/ext/matrix_uint3x3.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_uint3x3 RBLXMATH_EXT_matrix_uint3x3
/// @ingroup ext
///
/// Include <rblxmath/ext/matrix_uint3x3.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat3x3.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_uint3x3 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_uint3x3
	/// @{

	/// Unsigned integer 3x3 matrix.
	///
	/// @see ext_matrix_uint3x3
	typedef mat<3, 3, uint, defaultp>	umat3x3;

	/// Unsigned integer 3x3 matrix.
	///
	/// @see ext_matrix_uint3x3
	typedef mat<3, 3, uint, defaultp>	umat3;

	/// @}
}//namespace rblxmath
