/// @ref ext_matrix_int4x3
/// @file rblxmath/ext/matrix_int4x3.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_int4x3 RBLXMATH_EXT_matrix_int4x3
/// @ingroup ext
///
/// Include <rblxmath/ext/matrix_int4x3.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat4x3.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_int4x3 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_int4x3
	/// @{

	/// Signed integer 4x3 matrix.
	///
	/// @see ext_matrix_int4x3
	typedef mat<4, 3, int, defaultp>	imat4x3;

	/// @}
}//namespace rblxmath
