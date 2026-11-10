/// @ref ext_matrix_uint2x4
/// @file rblxmath/ext/matrix_uint2x4.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_uint2x4 RBLXMATH_EXT_matrix_int2x4
/// @ingroup ext
///
/// Include <rblxmath/ext/matrix_uint2x4.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat2x4.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_uint2x4 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_uint2x4
	/// @{

	/// Unsigned integer 2x4 matrix.
	///
	/// @see ext_matrix_uint2x4
	typedef mat<2, 4, uint, defaultp>	umat2x4;

	/// @}
}//namespace rblxmath
