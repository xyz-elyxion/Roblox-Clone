/// @ref ext_matrix_uint4x3
/// @file linearmath/ext/matrix_uint4x3.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_uint4x3 LINEARMATH_EXT_matrix_uint4x3
/// @ingroup ext
///
/// Include <linearmath/ext/matrix_uint4x3.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat4x3.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_matrix_uint4x3 extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_matrix_uint4x3
	/// @{

	/// Unsigned integer 4x3 matrix.
	///
	/// @see ext_matrix_uint4x3
	typedef mat<4, 3, uint, defaultp>	umat4x3;

	/// @}
}//namespace linearmath
