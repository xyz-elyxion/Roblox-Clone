/// @ref ext_matrix_int2x3
/// @file linearmath/ext/matrix_int2x3.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_int2x3 LINEARMATH_EXT_matrix_int2x3
/// @ingroup ext
///
/// Include <linearmath/ext/matrix_int2x3.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat2x3.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_matrix_int2x3 extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_matrix_int2x3
	/// @{

	/// Signed integer 2x3 matrix.
	///
	/// @see ext_matrix_int2x3
	typedef mat<2, 3, int, defaultp>	imat2x3;

	/// @}
}//namespace linearmath
