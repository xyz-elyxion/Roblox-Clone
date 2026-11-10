/// @ref ext_matrix_int4x4
/// @file rblxmath/ext/matrix_int4x4.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_int4x4 RBLXMATH_EXT_matrix_int4x4
/// @ingroup ext
///
/// Include <rblxmath/ext/matrix_int4x4.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat4x4.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_int4x4 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_int4x4
	/// @{

	/// Signed integer 4x4 matrix.
	///
	/// @see ext_matrix_int4x4
	typedef mat<4, 4, int, defaultp>	imat4x4;

	/// Signed integer 4x4 matrix.
	///
	/// @see ext_matrix_int4x4
	typedef mat<4, 4, int, defaultp>	imat4;

	/// @}
}//namespace rblxmath
