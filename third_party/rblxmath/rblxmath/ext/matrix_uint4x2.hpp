/// @ref ext_matrix_uint4x2
/// @file rblxmath/ext/matrix_uint4x2.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_uint4x2 RBLXMATH_EXT_matrix_uint4x2
/// @ingroup ext
///
/// Include <rblxmath/ext/matrix_uint4x2.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat4x2.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_uint4x2 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_uint4x2
	/// @{

	/// Unsigned integer 4x2 matrix.
	///
	/// @see ext_matrix_uint4x2
	typedef mat<4, 2, uint, defaultp>	umat4x2;

	/// @}
}//namespace rblxmath
