/// @ref ext_matrix_uint2x2
/// @file rblxmath/ext/matrix_uint2x2.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_uint2x2 RBLXMATH_EXT_matrix_uint2x2
/// @ingroup ext
///
/// Include <rblxmath/ext/matrix_uint2x2.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat2x2.hpp"

#if RBLXMATH_MESSAGES == RBLXMATH_ENABLE && !defined(RBLXMATH_EXT_INCLUDED)
#	pragma message("RBLXMATH: RBLXMATH_EXT_matrix_uint2x2 extension included")
#endif

namespace rblxmath
{
	/// @addtogroup ext_matrix_uint2x2
	/// @{

	/// Unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2
	typedef mat<2, 2, uint, defaultp>	umat2x2;

	/// Unsigned integer 2x2 matrix.
	///
	/// @see ext_matrix_uint2x2
	typedef mat<2, 2, uint, defaultp>	umat2;

	/// @}
}//namespace rblxmath
