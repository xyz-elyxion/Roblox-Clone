/// @ref ext_matrix_int2x2
/// @file linearmath/ext/matrix_int2x2.hpp
///
/// @see core (dependence)
///
/// @defgroup ext_matrix_int2x2 LINEARMATH_EXT_matrix_int2x2
/// @ingroup ext
///
/// Include <linearmath/ext/matrix_int2x2.hpp> to use the features of this extension.
///
/// Defines a number of matrices with integer types.

#pragma once

// Dependency:
#include "../mat2x2.hpp"

#if LINEARMATH_MESSAGES == LINEARMATH_ENABLE && !defined(LINEARMATH_EXT_INCLUDED)
#	pragma message("LINEARMATH: LINEARMATH_EXT_matrix_int2x2 extension included")
#endif

namespace linearmath
{
	/// @addtogroup ext_matrix_int2x2
	/// @{

	/// Signed integer 2x2 matrix.
	///
	/// @see ext_matrix_int2x2
	typedef mat<2, 2, int, defaultp>	imat2x2;

	/// Signed integer 2x2 matrix.
	///
	/// @see ext_matrix_int2x2
	typedef mat<2, 2, int, defaultp>	imat2;

	/// @}
}//namespace linearmath
